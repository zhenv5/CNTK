#pragma once

#include <set>
#include <unordered_map>
#include <random>
#include <opencv2/opencv.hpp>

#include "InnerInterfaces.h"
#include "ConcStack.h"

namespace Microsoft { namespace MSR { namespace CNTK {

    class ConfigParameters;

    class BaseTransformer : public Transformer
    {
    public:
        BaseTransformer(
            TransformerPtr next,
            const std::set<std::wstring>& appliedStreams,
            unsigned int seed);

        virtual void SetEpochConfiguration(const EpochConfiguration& config) override;
        virtual std::vector<InputDescriptionPtr> GetInputs() const override;
        virtual SequenceData GetNextSequence() override;

    protected:
        using UniRealT = std::uniform_real_distribution<double>;
        using UniIntT = std::uniform_int_distribution<int>;

        Sequence Apply(Sequence& mat);
        virtual void Apply(cv::Mat& mat) = 0;
        unsigned int GetSeed() const
        {
            return m_seed;
        }

        conc_stack<std::unique_ptr<std::mt19937>> m_rngs;

    private:
        std::set<std::wstring> m_appliedStreams;
        std::vector<bool> m_appliedStreamsHash;
        TransformerPtr m_next;
        unsigned int m_seed;
    };

    class CropTransformNew : public BaseTransformer
    {
    public:
        CropTransformNew(
            TransformerPtr next,
            const std::set<std::wstring>& appliedStreams,
            const ConfigParameters& parameters,
            unsigned int seed);

    protected:
        virtual void Apply(cv::Mat& mat) override;

    private:
        enum class CropType { Center = 0, Random = 1 };
        enum class RatioJitterType
        {
            None = 0,
            UniRatio = 1,
            UniLength = 2,
            UniArea = 3
        };

        void InitFromConfig(const ConfigParameters & config);
        CropType ParseCropType(const std::string& src);
        RatioJitterType ParseJitterType(const std::string& src);
        cv::Rect GetCropRect(CropType type, int crow, int ccol, double cropRatio, std::mt19937& rng);

        CropType m_cropType;
        double m_cropRatioMin;
        double m_cropRatioMax;
        RatioJitterType m_jitterType;
        bool m_hFlip;
    };

    class ScaleTransform : public BaseTransformer
    {
    public:
        ScaleTransform(TransformerPtr next,
            const std::set<std::wstring>& appliedStreams,
            unsigned int seed,
            int dataType,
            const ConfigParameters& config);

    private:
        void InitFromConfig(const ConfigParameters& config);
        virtual void Apply(cv::Mat& mat) override;


        int m_dataType;

        using StrToIntMapT = std::unordered_map<std::string, int>;
        StrToIntMapT m_interpMap;
        std::vector<int> m_interp;

        size_t m_imgWidth;
        size_t m_imgHeight;
        size_t m_imgChannels;
    };
}}}